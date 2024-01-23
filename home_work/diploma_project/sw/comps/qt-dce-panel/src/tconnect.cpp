#include "tconnect.h"
#include "psrgen.h"
#include <QDebug>
#include <QTime>

#define FACTOR 640

#define		DO_LOG

TConnect::TConnect(QObject *parent) : QObject(parent)
{

    future_psp_numbers = new QList<int>();
    smrand(3278);
    generator = new PSRGen() ;
    generator->set_bit_range(5);
    generator->set_clock(1000);
    //    for(int i = 0; i < 1000; i++){
    //            qDebug() << "Next psp number= " << generator->next() ;
    //   }

    connect(this, &TConnect::connectMode, this, &TConnect::setDuplexMode);
    mSending = false;
    mRecieving=true;
    for(int ii=0;ii<32;ii++){
        marks[0][ii] = false;
        marks[1][ii] = false;
    }

}

TConnect::~TConnect()
{
    delete generator;
}

void TConnect::setAdapt(TAdaptSpeed* adapt)
{
    ads = adapt;
    stat_result = ads->getStatResult();
}

void TConnect::setAdaptFreq(TAdaptFreq* adapt)
{
    adfreq = adapt;
    stat_reject = adfreq->getStatArray();
}

void TConnect::setParams(int addr , quint32 freqs_number,connect_Pars_T *pars)
{
    quint8 sinc_time = (quint8)((freqs_number * 40)/32);
    state.id		= CALL_WAITING_STATE;
    state.time		= 0;
    duplex_state.id		= CALL_WAITING_STATE;
    duplex_state.time = 0;
    adapt_speed.id		= NO_ADAPT;
    common_adapt_state.id = NO_ADAPTATION;
    adapt_speed.time = 0;
    parsm = pars;
    info_head_type = SPEED_ADAPTATION;
    info_head_par1 = 0;
    info_head_par2 = 0;

    s_reg.synchro_query_trans_time	= sinc_time;				// 800 посылок
    s_reg.synchro_trans_time			= sinc_time;				// 800 посылок
    s_reg.call_trans_time				= CONNECT_TIME_NO_NRCH;
    s_reg.call_confirm_trans_time		= CONNECT_TIME_NO_NRCH;
    s_reg.synchro_waiting_time		= sinc_time;				// 800 посылок
    s_reg.call_confirm_waiting_time	= CONNECT_TIME_NO_NRCH;
    s_reg.data_waiting_time			= 255;				// 5100 посылок
    s_reg.disconnect_confirm_waiting_time = 10;
    s_reg.disconnect_confirm_time = 2;
    s_reg.synchro_shift				= (quint16)(160 + 320*(addr & 0x03));

    enabled								= 1;
    address								= 0;
    rsSum									= 0;
    txCode									= 0;

    //инициализировать исходные значения для автономной проверки установки синхронизации
    s_reg.addressm						= (quint16)addr;			// поле УН (условный номер АПД)
    //slot.number.word64[0].word32[0]		= 10000;

//    for(int i=0; i < 128; i++)
//    {
//        rxChannels[i]	= ((quint8)i + (quint8)ch_offset) % ch_all;
//        txChannels[i]	= ((quint8)i + (quint8)ch_offset) % ch_all;
//    }

    //prs_struct_init( cfg );

    memset(rsErrors, 0, 128*8);								// Обнуление массива
    memset(rxCode, 0, 4*sizeof(quint32));
    memset(chDl, 0, 2);

    QString fname = QString("connect_log%1.txt").arg(s_reg.addressm);
    #ifdef	DO_LOG
        tcong = newLogger(fname.toUtf8().data(),1,1);
    #else
        tcong = newLogger(fname.toUtf8().data(),1,0);
    #endif
     strToFile(tcong,1,tr("class TConnect").toUtf8().data(),1);
     set_info_head_type(SPEED_ADAPTATION,0,0);
}

void TConnect::setConnectTime(int num)
{
    s_reg.call_trans_time				= num;
    s_reg.call_confirm_trans_time		= num;
    s_reg.call_confirm_waiting_time	= num;
}
/*******************************************************************************
 *  Функция создания кодограммы запроса сетевой синхронизации (клиент -> сервер)
 ******************************************************************************/
void TConnect::sos_frame_create(void* data,  int ind)
{
    frame_T *pframe = (frame_T *)data;
    qint32 v_slot;

    pframe->sos.id = SOS_FRAME_ID  ;
    quint16 tmp = (quint16)(state.time);
    pframe->sos.fr_num_older = (tmp >> 8)&0x0f;
    pframe->sos.fr_num_minor = tmp &0xff;
    pframe->sos.sender_address    = (uint16_t) (ind) ;
    pframe->sos.recipient_address    = server_address;
    pframe->sos.fake_insert = 0x1fffffff;

    v_slot = generator->clock().field[0];
    v_slot = v_slot*FACTOR;    //перевод времени в 1/3 мс
    pframe->sos.counter = v_slot;  // t0

    sincro_time.out_counter[state.time] = v_slot;		//хранится история времен по отправлению кадров

//    intToFile(tcong, 1, tr(" sos.id= ").toUtf8().data(),pframe->sos.id, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" adr1=").toUtf8().data() ,pframe->sos.sender_address, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" adr2= ").toUtf8().data(),pframe->sos.recipient_address, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" state.time=").toUtf8().data() ,tmp, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" word32[0]= ").toUtf8().data(),(qint32)(*slotTime).word32[0], 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" state.time= ").toUtf8().data(),state.time, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" t0= ").toUtf8().data(),pframe->sos.counter, 0, 10,' ',1);
}

void TConnect::sos_frame_analisys(frame_T *frame_ptr, int best_fr)  // сервер
{
    int32_t	v_slot;
    quint16 tmp = frame_ptr->sos.fr_num_older;
    if( frame_ptr->sos.recipient_address == s_reg.addressm )
    {
        server_address		= frame_ptr->sos.recipient_address;
        master = frame_ptr->sos.sender_address;
        sincro_time.in_frame_num		= ((tmp << 8)&0xf00) + frame_ptr->sos.fr_num_minor;
        sincro_time.in_counter	= 0;
        sincro_time.in_counter = frame_ptr->sos.counter;  //t0
        sincro_time.in_min_err_offset	= (best_fr*FACTOR)/320;
        v_slot = generator->clock().field[0];

        sincro_time.in_now_counter = v_slot * FACTOR; //перевод времени в 1/3 мс
        sincro_time.in_now_counter = sincro_time.in_now_counter + sincro_time.in_min_err_offset;  //t1
//        intToFile(tcong, 1, tr(" sender_address=").toUtf8().data(), master, 0, 10,' ',0);
//        intToFile(tcong, 0, tr(" in_frame_num= ").toUtf8().data(),sincro_time.in_frame_num, 0, 10,' ',1);
//        intToFile(tcong, 0, tr(" t0= ").toUtf8().data(),sincro_time.in_counter, 0, 10,' ',0);
//        intToFile(tcong, 0, tr(" t1= ").toUtf8().data(),v_slot * FACTOR, 0, 10,' ',0);
//        intToFile(tcong, 0, tr(" offset= ").toUtf8().data(),sincro_time.in_min_err_offset, 0, 10,' ',0);
//        intToFile(tcong, 0, tr(" t1_plus_offset= ").toUtf8().data(),sincro_time.in_now_counter, 0, 10,' ',1);
        state.id			= SOS_RECEIVING_STATE;
        state.time			= ((tmp << 8)&0xf00) + frame_ptr->sos.fr_num_minor;
    }
    else
    {
        // пойман запрос синхронизации к другому абоненту
        state.id			= ALIEN_SOS_RECEIVING_STATE;
        state.time			= ((tmp << 8)&0xf00) + frame_ptr->sos.fr_num_minor;													//пришла кдг для другого абонента
    }
}

void TConnect::sinchro_frame_create(void *data)   // сервер
{
    qint32	 counter;				//текущее время
    qint32	v_slot;
    qint32		v_time,send_time,send_time1,send_time2;
    frame_T		*pframe	= (frame_T *)data;

    uint16_t tmp = sincro_time.in_frame_num;  // номер слота, на котором был приём;
    pframe->sos.id = SINCHRO_FRAME_ID;
    pframe->sos.fr_num_older = (tmp >> 8)&0x0f;
    pframe->sos.fr_num_minor = tmp &0xff;

    pframe->sos.sender_address    = master;
    pframe->sos.recipient_address = server_address;
    pframe->sos.fake_insert = 0x1fffffff;

    v_slot = generator->clock().field[0];

    counter = v_slot * FACTOR;	//перевод времени в 1/3 мс  //t2

    send_time1 = (qint32)(sincro_time.in_counter - sincro_time.in_now_counter);	//  t0-t1

    send_time2 = send_time1*2; //  2*(t0-t1)

    v_time =  (qint32)(counter) - sincro_time.in_now_counter;	// t2-t1

    send_time = send_time2 - v_time;	// 2*(t0-t1) - (t2-t1)

    pframe->sos.counter	= send_time;

    sincro_time.out_counter[state.time] = counter;	//t2			// хранится история времен по отправлению кадров

    //intToFile(tcong, 1, tr(" sos.id= ").toUtf8().data(),pframe->sos.id, 0, 10,' ',0);
    //intToFile(tcong, 0, tr(" rec_address=").toUtf8().data(),address, 0, 10,' ',0);
    //intToFile(tcong, 0, tr(" in_frame_num=").toUtf8().data(),tmp, 0, 10,' ',0);
//    intToFile(tcong, 1, tr(" v_slot= ").toUtf8().data(),(qint32) (*slotTime).word32[0], 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" t2= ").toUtf8().data(),counter, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" state.time= ").toUtf8().data(),state.time, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" t2-t1= ").toUtf8().data(),v_time, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" t0-t1= ").toUtf8().data(),send_time1, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" 2*(t0-t1)= ").toUtf8().data(),send_time2, 0, 10,' ',0);
//    intToFile(tcong, 0, tr(" 2*(t0-t1)-(t2-t1)= ").toUtf8().data(),send_time, 0, 10,' ',1);

}

void TConnect::sinchro_frame_analisys(frame_T *frame_ptr, int best_fr)   // клиент
{
    qint32		counter, v_time, res_time;
    qint32		    v_in_counter;
    quint16		rem;
    qint32		    v_slot;
    quint16 tmp = frame_ptr->sos.fr_num_older;
    //intToFile(tcong, 1, tr(" frame_ptr->sos.recipient_address= ").toUtf8().data(),frame_ptr->sos.recipient_address, 0, 10,' ',0);
    //intToFile(tcong, 0, tr(" server_address= ").toUtf8().data(),server_address, 0, 10,' ',0);
    if ((frame_ptr->sos.id == SINCHRO_FRAME_ID)&&(frame_ptr->sos.recipient_address == server_address))
    {
        //intToFile(tcong, 0, tr(" frame_ptr->sos.id= ").toUtf8().data(),frame_ptr->sos.id, 0, 10,' ',1);

        sincro_time.in_counter	= 0;
        sincro_time.in_counter = frame_ptr->sos.counter; // 2*(t0-t1) - (t2-t1)

        v_in_counter = sincro_time.in_counter;
        //intToFile(tcong, 0, tr(" 2*(t0-t1)-(t2-t1)= ").toUtf8().data(), v_in_counter, 0, 10,' ',1);

        sincro_time.in_frame_num		= ((tmp << 8)&0xf00) + frame_ptr->sos.fr_num_minor;
        //intToFile(tcong, 0, tr(" in_frame_num= ").toUtf8().data(),sincro_time.in_frame_num, 0, 10,' ',1);

        sincro_time.in_min_err_offset	= (best_fr*FACTOR)/320;
        //*   прибавляем in_min_err_offset в миллисекундах - время начала сигнала          */
        //*   относительно начала захвата
        //intToFile(tcong, 0, tr(" best_fr= ").toUtf8().data(),best_fr, 0, 10,' ',1);

        v_slot = generator->clock().field[0];
        sincro_time.in_now_counter = v_slot * FACTOR+sincro_time.in_min_err_offset;	//перевод времени приёма в 1/3 мс  //t3
        //intToFile(tcong, 0, tr(" t3= ").toUtf8().data(),sincro_time.in_now_counter, 0, 10,' ',1);

        v_time = sincro_time.in_now_counter - sincro_time.out_counter[sincro_time.in_frame_num]; //вычисление dT2 = t3-t0
        //intToFile(tcong, 0, tr(" t3-t0= ").toUtf8().data(),v_time, 0, 10,' ',1);

        res_time = v_time + v_in_counter; //сложение dT2 и времени, принятого от сервера = (t3-t0)+(2*(t0-t1)-(t2-t1))
        //intToFile(tcong, 0, tr(" (t3-t0)+(2*(t0-t1)-(t2-t1))= ").toUtf8().data(),res_time, 0, 10,' ',1);

        res_time = res_time/2;  //деление (2*dT0) на 2
        //intToFile(tcong, 0, tr(" res_time= ").toUtf8().data(),res_time, 0, 10,' ',1);

        counter = sincro_time.in_now_counter - res_time;  //вычитаем из текущего времени dT0
        //intToFile(tcong, 0, tr(" t3-res_time= ").toUtf8().data(),counter, 0, 10,' ',1);

        v_time = counter / FACTOR;
        rem = counter % FACTOR;  //перевод 1/3 мс в слоты
        //intToFile(tcong, 0, tr(" v_time= ").toUtf8().data(),v_time, 0, 10,' ',1);
        //intToFile(tcong, 0, tr(" rem= ").toUtf8().data(),rem, 0, 10,' ',0);

        if( rem > 159 )
           s_reg.synchro_shift	-= 5*(rem - 160);
        else
            s_reg.synchro_shift	+= 5*(160 - rem);

        //intToFile(tcong, 0, tr(" synchro_shift = ").toUtf8().data(),s_reg.synchro_shift, 0, 10,' ',1);

        slot.number.word32[0]				= (quint32)v_time;
        set_mdm_time(v_time - FUTURE_PSP);
        state.id = SINCHRO_RECEIVING_STATE;
    }
    else
    {
        // получение времени другим абонентом
        state.id			=ALIEN_SINCHRO_RECEIVING_STATE;
        state.time			= ((tmp << 8)&0xf00) + frame_ptr->sos.fr_num_minor;
    }
}

void TConnect::info_head_create(void* data, int htype, quint8 par1, quint8 par2)
{
    head_Info *ihead = (head_Info *)data;
    switch (htype)
    {
    case SPEED_ADAPTATION:
        ihead->speed_adapt.id = htype;
        ihead->speed_adapt.tspeed=par1;
        ihead->speed_adapt.rspeed=par2;
        ads->renewSpeedNumberArray(par1,par2);
        break;
    case  ADDITIONAL_FREQUENCY_SHIFT:
        ihead->add_shift.id=htype;
        ihead->add_shift.f_number=par1;
        ihead->add_shift.f_shift=par2;
        break;
    case  FREQUENCY_ADAPTATION:
        ihead->f_adapt.id=htype;
        ihead->f_adapt.f_number1=par1;
        ihead->f_adapt.f_number2=par2;
        break;
    case  CONNECTION_BREAK:
        ihead->connect_down.id=htype;
        ihead->connect_down.cap1=par1;
        ihead->connect_down.cap2=par2;
        if (parsm->break_enabled){
            state.id = SEND_DISCONNECT_REQUEST;
            state.time=0;
        }
        break;
    }
}
/*******************************************************************************
 *  Функция создания кодограммы запроса соединения (клиент )
 ******************************************************************************/
void TConnect::call_frame_create(void* data)
{
    frame_T *pframe = (frame_T *)data;
    uint16_t tmp = (uint16_t)(state.time);
    pframe->call.id = CALL_FRAME_ID;
    pframe->call.fr_num_older = (tmp >> 8)&0x0f;
    pframe->call.fr_num_minor = tmp &0xff;
    pframe->call.recipient_address	= server_address;
    pframe->call.sender_address		= s_reg.addressm;
    pframe->call.fake_insert = 0x1fffffff;
    if (parsm->switch_mode==0)
    {
        parsm->fixFrequencyMode = true;
        pframe->call.chan_number[0]=parsm->fixFrequencies[0];
        pframe->call.chan_number[1]=parsm->fixFrequencies[1];
    }else{
        parsm->fixFrequencyMode = false;
        pframe->call.chan_number[0]=parsm->chan_number[0];
        pframe->call.chan_number[1]=parsm->chan_number[1];
    }
    parsm->txChan = pframe->call.chan_number[0];
    parsm->rxChan = pframe->call.chan_number[1];

    if (parsm->duplex) {
        parsm->simplex = 0;
    }
    pframe->call.band = parsm->band;
    pframe->call.speed= parsm->speed;
    pframe->call.switch_mode = parsm->switch_mode;
    pframe->call.rs1 = parsm->rs1;
    pframe->call.duplex = parsm->duplex;
    pframe->call.break_enabled = parsm->break_enabled;
    pframe->call.auto_end=parsm->break_on_50;
    parsm->half_duplex_num =1;
}

int TConnect::call_frame_analisys(frame_T *frame_ptr) // сервер прием запроса на соединение
{
    int res=0;
    quint16 tmp = frame_ptr->call.fr_num_older;

    if( frame_ptr->call.recipient_address == s_reg.addressm )
    {
        klient_address			= frame_ptr->call.sender_address;
        parsm->startup = false;
        server_address = frame_ptr->call.recipient_address;

        state.time = ((tmp << 8)&0xf00) + frame_ptr->call.fr_num_minor;
        parsm->duplex = frame_ptr->call.duplex;
        if (parsm->duplex == 1) {
            parsm->simplex = 0;
            duplex_state.id		= CALL_CONFIRM_TRANSMISSION_STATE; // сразу при дуплексе передача подтверждения
            duplex_state.time		=state.time;
            is_kdg_to_transmit = true;
        }else{
            state.id			= CALL_RECEIVING_STATE;
        }
        parsm->break_enabled = frame_ptr->call.break_enabled;
        parsm->break_on_50 = frame_ptr->call.auto_end;
        parsm->band = frame_ptr->call.band;
        parsm->speed = frame_ptr->call.speed;
        parsm->switch_mode = frame_ptr->call.switch_mode;
        parsm->rs1 = frame_ptr->call.rs1;
        parsm->wait_best_frequency_in_transmission = false;
        if (parsm->switch_mode==1) setConnectTime(CONNECT_TIME_NRCH);
        else  setConnectTime(CONNECT_TIME_NO_NRCH);

        if (parsm->switch_mode==0){
            parsm->server_num = klient_address;
            parsm->fixFrequencyMode = true;
            parsm->fixFrequencies[0] = frame_ptr->call.chan_number[1];
            parsm->fixFrequencies[1] = frame_ptr->call.chan_number[0];
        }else{
            parsm->fixFrequencyMode = false;
            parsm->chan_number[0] = frame_ptr->call.chan_number[1];
            parsm->chan_number[1] = frame_ptr->call.chan_number[0];
        }
        parsm->txChan = frame_ptr->call.chan_number[1];
        parsm->rxChan = frame_ptr->call.chan_number[0];
        parsm->half_duplex_num = 2;
        res = 1;
    }
    else{    //пришла кдг для другого абонента
        state.id			= ALIEN_CALL_RECEIVING_STATE;
        state.time			= ((tmp << 8)&0xf00) + frame_ptr->call.fr_num_minor;
    }
    //logConnectParams(parsm);
    return res;
}

int TConnect::call_confirm_frame_analisys(frame_T *frame_ptr) // клиент -анализ подтверждения соединения
{
    int res=0;
    if (frame_ptr->call.id == CALL_CONFIRM_FRAME_ID)
    {
        if( frame_ptr->call.recipient_address == s_reg.addressm ){
            state.id = CALL_CONFIRM_RECEIVING_STATE;
            if(parsm->switch_mode == 1)
            {
                if (parsm->duplex == 1 )
                    duplex_state.id = CALL_CONFIRM_RECEIVING_STATE;
            }
            klient_address = frame_ptr->call.recipient_address;
            server_address			= frame_ptr->call.sender_address;
            parsm->break_enabled = frame_ptr->call.break_enabled;
            parsm->break_on_50 = frame_ptr->call.auto_end;
            parsm->band = frame_ptr->call.band;
            parsm->speed = frame_ptr->call.speed;
            parsm->switch_mode = frame_ptr->call.switch_mode;
            parsm->duplex = frame_ptr->call.duplex;
            parsm->half_duplex_num =1;
            if(parsm->switch_mode == 0){
                parsm->server_num = server_address;
            }
            if(parsm->switch_mode == 1){
                if(frame_ptr->call.chan_number[0]<255){
                    parsm->fixFrequencies[0] = frame_ptr->call.chan_number[0];
                    if (parsm->duplex == 1 )
                    {
                        if(no_nrch ) emit calcNrch();
                        no_nrch = false;
                    }
                }
            }
//            intToFile(tcong, 1, tr(" nrch1= ").toUtf8().data(),frame_ptr->call.chan_number[0], 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" state.id= ").toUtf8().data(),state.id, 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" state.time= ").toUtf8().data(),state.time, 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" dstate.id= ").toUtf8().data(),duplex_state.id, 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" dstate.time= ").toUtf8().data(),duplex_state.time, 0, 10,' ',1);
//            parsm->fixFrequencies[1] = frame_ptr->call.chan_number[1];

            res = 1;
        }
        else{
            // подтверждение соединения другому абоненту
            state.id			=ALIEN_CALL_CONFIRM_RECEIVING_STATE;
            quint16 tmp = frame_ptr->call.fr_num_older;
            state.time			= ((tmp << 8)&0xf00) + frame_ptr->sos.fr_num_minor;
        }
    }
    return res;
}
/*******************************************************************************
 *  Функция создания кодограммы ответа на запрос соединения ( сервер ->клиент )
 ******************************************************************************/
void TConnect::call_confirm_frame_create(void* data)
{
    frame_T *pframe = (frame_T *)data;
    uint16_t tmp = (uint16_t)(state.time);  
    pframe->call.id = CALL_CONFIRM_FRAME_ID;
    pframe->call.fr_num_older = (tmp >> 8)&0x0f;
    pframe->call.fr_num_minor = tmp &0xff;
    pframe->call.recipient_address	= klient_address;
    pframe->call.sender_address		= s_reg.addressm;
    if (parsm->switch_mode == 1){
        if (parsm->search_nrch_mode) pframe->call.chan_number[0] = 255;
        else pframe->call.chan_number[0] = parsm->fixFrequencies[1];
    }else{
        pframe->call.chan_number[0] = parsm->chan_number[1];
    }
    pframe->call.chan_number[1] = parsm->chan_number[0];
    pframe->call.band = parsm->band;
    pframe->call.speed= parsm->speed;
    pframe->call.switch_mode = parsm->switch_mode;
    pframe->call.duplex = parsm->duplex;
    pframe->call.fake_insert = 0x1fffffff;
    pframe->call.break_enabled = parsm->break_enabled;
    pframe->call.auto_end=parsm->break_on_50;
}

void TConnect::setRecvNrchFreqNumber(int num1,int num2)
{
       parsm->fixFrequencies[1] = num1;
       parsm->fixFrequencies[2] = num2;
       parsm->search_nrch_mode = false;
       if (parsm->duplex==1){
           duplex_state.time=3;
           state.time=3;
       }
       intToFile(tcong, 1, tr(" Modem ").toUtf8().data(),s_reg.addressm, 0, 10,' ',0);
       intToFile(tcong, 0, tr(" fixFrequency1 = ").toUtf8().data(),num1, 10, 10,' ',0);
       intToFile(tcong, 0, tr(" fixFrequency2 = ").toUtf8().data(),num2, 10, 10,' ',1);
}

void TConnect::start_synchro(connect_Pars_T *pars)
{
    state.id		= SOS_TRANSMISSION_STATE;
    state.time		= 20 * s_reg.synchro_query_trans_time;
    server_address		= pars->server_num;
    is_kdg_to_transmit = true;
}

void TConnect::start_connect(connect_Pars_T *pars)
{
    state.id		= CALL_TRANSMISSION_STATE;
    state.time		= 20 * s_reg.call_trans_time;
    server_address		= pars->server_num;
    pars->startup = true;
    parsm = pars;
    if (parsm->duplex == 1){
        duplex_state.id = CALL_CONFIRM_WAITING_STATE;
        duplex_state.time = 20*s_reg.call_confirm_waiting_time;
    }
    parsm->wait_best_frequency_in_transmission = false;
    is_kdg_to_transmit = true;
    no_nrch = true;
}

bool TConnect::generatingServiceKdg()
{
    return is_kdg_to_transmit;
}

bool TConnect::getEitherStatValid(int index,bool *good_chan)
{
    bool res_s = false;
    bool res_f = false;
    bool ok1=false;
    bool ok2=false;
    if (parsm->speed_adapt)
        res_s = ads->getStatValid(index,&ok1);
    if (parsm->freq_adapt)
        res_f = adfreq->getStatValid(index,&ok2);
    if (parsm->fixFrequencyMode)
        ok1 = true;
    *good_chan = ok1||ok2;
    return (res_s||res_f);
}

bool TConnect::stats_for_send(int pos)
{
    bool good_ch=false;
    if (getEitherStatValid(pos, &good_ch)){
        //log_good(good_ch);
    }
    return good_ch;
}

void TConnect::log_good(bool st)
{
    int res=0;
    int len = parsm->futureSendIndexes1.count();
    if (st){
        strToFile(tcong,0,tr(" ============== ").toUtf8().data(),1);
        strToFile(tcong,1,tr(" indexF= ").toUtf8().data(),0);
        for (int ii=0;ii<len;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),ii, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" futurF= ").toUtf8().data(),0);
        for (int ii=0;ii<len;ii++) {
            int jj = parsm->futureSendIndexes1.at(ii);
            intToFile(tcong, 0, tr("  ").toUtf8().data(),jj, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        if (parsm->speed_adapt){
            strToFile(tcong,1,tr(" Good sp ").toUtf8().data(),0);
            for(int jj=0;jj<parsm->startFreqNumber;jj++){
                res=0;
                for (int ii = 0; ii < stat_result[jj].rsgood.count(); ii++) {
                    res = res + stat_result[jj].rsgood.at(ii);
                }
                intToFile(tcong, 0, tr("  ").toUtf8().data(),res, 3, 10,' ',0);
            }
            strToFile(tcong,0,tr(" ").toUtf8().data(),1);
        }
        if (parsm->freq_adapt){
            strToFile(tcong,1,tr(" Good fr ").toUtf8().data(),0);
            for(int jj=0;jj<parsm->startFreqNumber;jj++){
                res=0;
                for (int ii = 0; ii < stat_reject[jj].rsgood.count(); ii++) {
                    res = res + stat_reject[jj].rsgood.at(ii);
                }
                intToFile(tcong, 0, tr("  ").toUtf8().data(),res, 3, 10,' ',0);
            }
            strToFile(tcong,0,tr(" ").toUtf8().data(),1);
        }
    }
}

bool TConnect::speed_validation_to_replace_freq(int fIndex)
{
    bool res=true;
    if (parsm->speed_adapt){
        if (parsm->txVariant[fIndex] == parsm->rxVariant[fIndex])
        {
            res = ads->getLowSpeedCondition(fIndex);
        }
    }
    return res;
}

bool TConnect::analysis_of_decoding_statistics(int sInd)
{
    bool reverseCome=false;
    int fIndex;
    //logFrequencySet(sInd);
    for(int jj=0;jj<FUTURE_PSP;jj++){
        fIndex = parsm->futureSendIndexes1.at(jj);
        if (fIndex != sInd)
            reverseCome = speed_stat_processing(fIndex,sInd);
        if (reverseCome) break;
    }
    return reverseCome;
}

void TConnect::analysis_of_decoding_fixed(int freqIndex,int freqIndex1)
{
    int newRxVar;
    int back =0;
    if (parsm->speed_adapt){
        if (stat_result[freqIndex].speed_advise > 0){
                newRxVar =ads->getNewSpeedNumberFix(freqIndex,&back);
                set_info_head_type(SPEED_ADAPTATION, freqIndex1, newRxVar);
                if ((back==1)||(back==2)){
                    int var = ads->getBaseIndex();
                    ads->setCurIndex(freqIndex,var);
                    parsm->newRxVariant = var;
                    parsm->newTxVariant = var;
                }else{
                    parsm->newRxVariant = newRxVar;
                }
                parsm->adaptFreqIndex = freqIndex;
                logSpeedSet(freqIndex);
                ads->clearStatistics(freqIndex);
        }
    }
}

void TConnect::setBaseMode(int fnum)
{
    int var = ads->getBaseIndex();
    ads->setCurIndex(fnum,var);
    if (parsm->txVariant[fnum] != var ) switchTxSpeed(fnum, var);
    if (parsm->rxVariant[fnum] != var ) switchRxSpeed(fnum, var);
}

bool TConnect::speed_stat_processing(int freqIndex,int sInd)
{
    bool res = false;
    int newRxVar;
    int back =0;
    if (parsm->speed_adapt){
        if (stat_result[freqIndex].speed_advise > 0){
            if (stat_result[freqIndex].speed_advise < REPLACE_FREQ)
            {
                res = true;
                newRxVar =ads->getNewSpeedNumber(freqIndex,&back);
                set_info_head_type(SPEED_ADAPTATION, freqIndex, newRxVar);
                if ((back==1)||(back==2)){
                    setBaseMode(freqIndex);
                    if (back==2) adfreq->allow_replace_freq(freqIndex,true);
                }else{
                    switchRxSpeed(freqIndex, newRxVar);
                    if (parsm->rxVariant[freqIndex] != newRxVar ) switchRxSpeed(freqIndex, newRxVar);
                    if (back==3) adfreq->allow_replace_freq(freqIndex,false);
                }
                logSpeedSet(sInd);
                ads->clearStatistics(freqIndex);
            }
        }
    }
    if (parsm->freq_adapt){
        if (stat_reject[freqIndex].reject){
            if ((stat_reject[freqIndex].allow_reject)||(!parsm->speed_adapt)){
                logFrequencySet(sInd);
                bool perm = speed_validation_to_replace_freq(freqIndex);
                //if (perm) renewFreqSet(freqIndex,stat_reject[freqIndex].freqNum);
                if (perm) renewFreqSet(freqIndex,parsm->fChannels[freqIndex]);
                set_info_head_type(FREQUENCY_ADAPTATION, freqIndex, parsm->fChannels[freqIndex]);
                res=true;
                marks[0][freqIndex] = true;
            }
        }
    }
    return res;
}

void TConnect::switchTxSpeed(int ch, int var)
{
    emit setNewTxSpeed(ch,var);
}

void TConnect::switchRxSpeed(int ch, int var)
{
    emit setNewRxSpeed(ch,var);
}

void TConnect::renewFreqSet(quint8 index,quint8 fNum)
{
    int newFreq = adfreq->changeFrequenciesSet(index,fNum);
    parsm->fChannels[index] = newFreq;
    setBaseMode(index);
    //ads->clearStatistics(index);
}

int TConnect::tx_heading(quint8* hbytes, int chanIndex, int chanIndex1)
{
    int res=2; // длина заголовка
    bool goodChan=stats_for_send(chanIndex);
    if (goodChan){
        if (parsm->fixFrequencyMode)
            analysis_of_decoding_fixed(chanIndex,chanIndex1);
        else
            analysis_of_decoding_statistics(chanIndex);
    }
     info_head_create(hbytes,info_head_type,info_head_par1,info_head_par2);
    return res;
}

void TConnect::rx_heading(char* hbytes,QList<int>* head,QList<int>* mess)
{
    //intToFile(tcong, 1, tr(" rx_heading state.id= ").toUtf8().data(),state.id, 0, 10,' ',1);
    if (state.id != DATA_RECEIVING_STATE){
        return;
    }
    static bool log = true;
    head_Info	*cap	= (head_Info *)hbytes;
    quint8 id = cap->speed_adapt.id;
    int fIndex = cap->f_adapt.f_number1;
    int value = cap->f_adapt.f_number2;

    if (id + fIndex + value == 0) return;

    if (head->count() == 3){
        if ((head->at(0) == id)&&(head->at(1)==fIndex)&&(head->at(2)==value)){
            return;
        }else{
//            intToFile(tcong, 1, tr(" rx_heading id= ").toUtf8().data(),id, 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" fIndex= ").toUtf8().data(),fIndex, 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" value= ").toUtf8().data(),value, 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" head count= ").toUtf8().data(),head->count(), 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" head 0= ").toUtf8().data(),head->at(0), 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" head 1= ").toUtf8().data(),head->at(1), 0, 10,' ',0);
//            intToFile(tcong, 0, tr(" head 2= ").toUtf8().data(),head->at(2), 0, 10,' ',1);
            head->clear();
            head->append(id);
            head->append(fIndex);
            head->append(value);
        }
    }

    *mess = mess->mid(2);
    for (int ii = head->count()-1; ii >=0; ii--) mess->prepend(head->at(ii));
    if(id != CONNECTION_BREAK)  breakConnectKdgNumber=0;
    if(id != FREQUENCY_ADAPTATION)  newFreqKdgNumber=0;

    switch (id)
    {
    case SPEED_ADAPTATION:
        //if (parsm->speed_adapt)
        {
            intToFile(tcong, 1, tr(" request new Speed for Index= ").toUtf8().data(),fIndex, 0, 10,' ',1);
            if ((fIndex < parsm->startFreqNumber)&&(ads->getValidCondition(value))){
                int oldVariant = parsm->txVariant[fIndex];
                int newVariant = value;
                if (newVariant != oldVariant){ // пришел запрос на замену скорости в передаче
                    intToFile(tcong, 1, tr(" request new Speed= ").toUtf8().data(),newVariant, 0, 10,' ',0);
                    intToFile(tcong, 0, tr(" for FreqIndex= ").toUtf8().data(), fIndex, 0, 10,' ',0);
                    intToFile(tcong, 0, tr(" FreqNumber= ").toUtf8().data(),parsm->fChannels[fIndex], 0, 10,' ',0);
                    intToFile(tcong, 0, tr(" old Variant= ").toUtf8().data(),oldVariant, 0, 10,' ',0);

                    switchTxSpeed(fIndex,newVariant);

                    ads->clearStatistics(fIndex);
                    intToFile(tcong, 0, tr(" new Variant= ").toUtf8().data(),newVariant, 0, 10,' ',0);
                    strToFile(tcong,0,tr(" speed request executed\n ").toUtf8().data(),1);
                }
            }
        }
        break;
    case ADDITIONAL_FREQUENCY_SHIFT:
        if (parsm->switch_mode != 0)
            parsm->fixFrequencies[0] = cap->add_shift.f_number;
        parsm->wait_best_frequency_in_transmission = false;
        emit connectStatus(server_address,klient_address, parsm->fixFrequencies[0], parsm->fixFrequencies[1]);
        //if (log) logConnectParams(parsm);
        log = false;
        break;
    case FREQUENCY_ADAPTATION:
        //if (parsm->freq_adapt)
        {
            if ((fIndex < parsm->startFreqNumber)&&(value < parsm->allFreqNumber)){
                if (newFreq != value) {
                    newFreqKdgNumber=0;
                    newFreq = value;
                    intToFile(tcong, 1, tr("  recv id= ").toUtf8().data(),cap->f_adapt.id, 0, 10,' ',0);
                    intToFile(tcong, 0, tr("  indFreq= ").toUtf8().data(),cap->f_adapt.f_number1, 0, 10,' ',0);
                    intToFile(tcong, 0, tr("  newFreq= ").toUtf8().data(),cap->f_adapt.f_number2, 0, 10,' ',1);
                    strToFile(tcong,0,tr(" ").toUtf8().data(),1);
                }

                int oldFreq = parsm->fChannels[fIndex];
                if (newFreq != oldFreq){ // пришел запрос на смену частоты
                    int change = adfreq->changeImplicitFreq(fIndex,oldFreq,newFreq);
                    intToFile(tcong, 1, tr(" request new freq index= ").toUtf8().data(),cap->f_adapt.f_number1, 0, 10,' ',0);
                    intToFile(tcong, 0, tr(" old freq= ").toUtf8().data(),parsm->fChannels[cap->f_adapt.f_number1], 0, 10,' ',0);
                    intToFile(tcong, 0, tr(" new freq= ").toUtf8().data(),cap->f_adapt.f_number2, 0, 10,' ',1);

                    if(change >= 0){
                        parsm->fChannels[fIndex] = newFreq;
                        setBaseMode(fIndex);
                        intToFile(tcong,1, tr("  fInd=").toUtf8().data(),fIndex, 3, 10,' ',0);
                        intToFile(tcong,0, tr("  fOld=").toUtf8().data(),oldFreq, 3, 10,' ',0);
                        intToFile(tcong,0, tr("  fNew=").toUtf8().data(),parsm->fChannels[fIndex], 3, 10,' ',0);
                        intToFile(tcong,0, tr("  txVar=").toUtf8().data(),parsm->txVariant[fIndex], 3, 10,' ',0);
                        intToFile(tcong,0, tr("  rxVar=").toUtf8().data(),parsm->rxVariant[fIndex], 3, 10,' ',0);
                        strToFile(tcong,0,tr(" freq request executed\n ").toUtf8().data(),1);
                        //ads->clearStatistics(fIndex);
                    }
                }
            }
        }
        break;
    case CONNECTION_BREAK:
        if (parsm->break_enabled){
            breakConnectKdgNumber++;
            if (breakConnectKdgNumber>2){
                state.id = RECV_DISCONNECT_REQUEST;
                state.time=0;
                log = true;
                breakConnectKdgNumber=0;
            }
        }
        break;
    }
}

void TConnect::freqsMarqing(bool state)
{
    if (state) strToFile(tcong,0,tr("  *  ").toUtf8().data(),0);
    else strToFile(tcong,0,tr("     ").toUtf8().data(),0);
}

void TConnect::logSpeedSet(int curf)
{
    bool ok;
    if (ads->getStatValid(curf,&ok))
    {
        intToFile(tcong, 1, tr(" SpeedSt for chan").toUtf8().data(),curf, 3, 10,' ',0);
        strToFile(tcong,0,tr(" errRS =").toUtf8().data(),0);
        for (int ii=0;ii<stat_result[curf].rsgood.count();ii++) {
            intToFile(tcong, 0, tr("").toUtf8().data(),stat_result[curf].rsgood.at(ii), 2, 10,' ',0);
        }
        intToFile(tcong, 0, tr(" slotCounter=").toUtf8().data(),slotCounter, 5, 10,' ',1);

        int len = parsm->futureSendIndexes1.count();
        strToFile(tcong,1,tr(" indexF= ").toUtf8().data(),0);
        for (int ii=0;ii<len;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),ii, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" futurF= ").toUtf8().data(),0);
        for (int ii=0;ii<len;ii++) {
            int jj = parsm->futureSendIndexes1.at(ii);
            intToFile(tcong, 0, tr("  ").toUtf8().data(),jj, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" rxSets= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),parsm->rxVariant[ii], 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" txSets= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),parsm->txVariant[ii], 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" speeMX= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_result[ii].summa_mx, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" speeRS= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_result[ii].summa_rs, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" advise= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_result[ii].speed_advise, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" allowr= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_reject[ii].allow_reject, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

    }
}


void TConnect::logFrequencySet(int curf)
{
    bool ok;
    if (adfreq->getStatValid(curf,&ok))
    {
        intToFile(tcong, 1, tr(" freqSta for chan").toUtf8().data(),curf, 3, 10,' ',0);
        strToFile(tcong,0,tr(" errRS =").toUtf8().data(),0);
        for (int ii=0;ii<stat_reject[curf].rsgood.count();ii++) {
            intToFile(tcong, 0, tr("").toUtf8().data(),stat_reject[curf].rsgood.at(ii), 2, 10,' ',0);
        }
        intToFile(tcong, 0, tr(" slotCounter=").toUtf8().data(),slotCounter, 5, 10,' ',1);

        int len = parsm->futureSendIndexes1.count();
        strToFile(tcong,1,tr(" indexF= ").toUtf8().data(),0);
        for (int ii=0;ii<len;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),ii, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" futurF= ").toUtf8().data(),0);
        for (int ii=0;ii<len;ii++) {
            int jj = parsm->futureSendIndexes1.at(ii);
            intToFile(tcong, 0, tr("  ").toUtf8().data(),jj, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" rxSets= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),parsm->rxVariant[ii], 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" txSets= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),parsm->txVariant[ii], 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" freSet= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),parsm->fChannels[ii], 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" freMrk=   ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            if (marks[0][ii])    freqsMarqing(true);
            else  freqsMarqing(false);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" freqMX= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_reject[ii].summa_mx, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" freqRS= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_reject[ii].summa_rs, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" allowr= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_reject[ii].allow_reject, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);

        strToFile(tcong,1,tr(" reject= ").toUtf8().data(),0);
        for (int ii=0;ii<parsm->startFreqNumber;ii++) {
            intToFile(tcong, 0, tr("  ").toUtf8().data(),stat_reject[ii].reject, 3, 10,' ',0);
        }
        strToFile(tcong,0,tr(" ").toUtf8().data(),1);
    }
}



void TConnect::addAdaptStat(int err, int max, int ind, int ch)
{

    if (parsm->speed_adapt){
        ads->addStat(err,max,ind,ch);
    }
    if(parsm->freq_adapt){
        adfreq->addStatFreq(err,max,ind,ch);
    }
}

bool TConnect::tx_process(quint8* bytes, int ind)
{
    if (!is_kdg_to_transmit) return false;
    else
    {
        for (int ii = 0; ii < CODOGRAMM_LEN; ii++) bytes[ii] = 0;
        switch (state.id)
        {
        case CALL_TRANSMISSION_STATE:
            call_frame_create(bytes);  // клиент
            break;
        case CALL_CONFIRM_RECEIVING_STATE:
            if (parsm->duplex == 1) call_frame_create(bytes);  // клиент
            break;
        case CALL_CONFIRM_TRANSMISSION_STATE:
            if (parsm->duplex != 1) {
                call_confirm_frame_create(bytes);  // сервер
            }
            break;
        case SOS_TRANSMISSION_STATE:
            sos_frame_create(bytes, ind);  // клиент
            break;
        case SINCHRO_TRANSMISSION_STATE:
            sinchro_frame_create(bytes); // сервер
            break;
        }
        switch (duplex_state.id)
        {
        case CALL_CONFIRM_TRANSMISSION_STATE:
            call_confirm_frame_create(bytes);  // сервер
            break;
        }
        //intToFile(tcong, 1, tr(" tx_process state.id= ").toUtf8().data(),state.id, 0, 10,' ',1);
        return true;
    }
}

quint32 TConnect::rx_process(char *bytes2, int bf)//, mdm_context_t* rxMod)
{
    frame_T		*pframe	= (frame_T *)bytes2;
    uint8_t		res		= 0;
    if (pframe->sos.fake_insert != 0x1fffffff) return res;
    switch (state.id)
    {
    case SINCHRO_WAITING_STATE:
        sinchro_frame_analisys(pframe,bf);   // клиент
        break;
    case CALL_CONFIRM_WAITING_STATE:
         if (parsm->duplex != 1){
            res=call_confirm_frame_analisys(pframe);  // клиент
         }
        qDebug() << "call_confirm_frame_analisys(pframe) = " <<res;
        break;
    case CALL_WAITING_STATE:
        switch( pframe->sos.id)
        {
        case SOS_FRAME_ID:
            sos_frame_analisys(pframe,bf);  // сервер
            break;
        case CALL_FRAME_ID:
            res = call_frame_analisys(pframe);    // сервер
            //qDebug() << "call_frame_analisys(pframe) = " <<res;
            break;
        }
        break;
    }
    switch (duplex_state.id)
    {
    case CALL_CONFIRM_WAITING_STATE:
    case CALL_CONFIRM_RECEIVING_STATE:
        res=call_confirm_frame_analisys(pframe);  // клиент
        break;
    }
    //intToFile(tcong, 1, tr(" rx_process state.id= ").toUtf8().data(),state.id, 0, 10,' ',1);
    return res;
}

void TConnect::clearBreakdown()
{
    if (state.id == DATA_RECEIVING_STATE){
        notDecodedSlots = 0;
    }
}

void TConnect::setConnectBreakdown()
{
    if (parsm->break_on_50 == 0) return;
    if (state.id == DATA_RECEIVING_STATE){
        notDecodedSlots++;
        if (notDecodedSlots > NO_SIGNAL_THRESHOLD) {
            state.id = CHANNEL_BREAKDOWN;
            state.time=0;
            notDecodedSlots = 0;
        }
    }
}

int TConnect::state_refresh()
{
    int		res	= 0;
    QString statusLine="";
    if (state.time == 0)
    {
        notDecodedSlots = 0;  // сброс счетчика не декодированных слотов при переходе в следующее состояние
        //intToFile(tcong, 1, tr(" state_refresh state.id= ").toUtf8().data(),state.id, 0, 10,' ',0);
        switch (state.id)				//Изменить состояние по истечении времени
        {
        case SOS_TRANSMISSION_STATE:
            state.id		= SINCHRO_WAITING_STATE;
            is_kdg_to_transmit = false;
            state.time		= s_reg.synchro_waiting_time * 20;
            break;

        case SINCHRO_TRANSMISSION_STATE:
            state.id		= CALL_WAITING_STATE;
            is_kdg_to_transmit = false;
            statusLine="";
            emit synchrStatus(statusLine);
            break;

        case CALL_TRANSMISSION_STATE:
            state.id		= CALL_CONFIRM_WAITING_STATE;
            is_kdg_to_transmit = false;
            state.time		= s_reg.call_confirm_waiting_time * 20;
            //emit connectStatus(mdmNumber, srcAdress, strStatus(0), msg);
            break;

        case SEND_DISCONNECT_REQUEST:
            endOfConnection("Соединение разорвано");
            break;

        case RECV_DISCONNECT_REQUEST:
            endOfConnection("Соединение разорвано");
            break;

        case CHANNEL_BREAKDOWN:
            if (speedAdaptElevated())
                breakTheSpeed("Сброс адаптации на низкую скорость");
            else
                endOfConnection("Канал авария");
            break;

        case SEND_DISCONNECT_CONFIRM:       // отправлено подтверждение на разрыв
//            disconnect_make(cfg);
//            emit disconnectStatus(srcAdress, mdmNumber,strStatus(3),msg);
//            strToFile(apdlog,1,tr("terminate").toUtf8().data(),1);
            break;
        case RECV_DISCONNECT_CONFIRM:                     // подтверждение на разрыв получен
//            disconnect_make(cfg);
//            emit disconnectStatus(mdmNumber, srcAdress,strStatus(2),msg);
//            strToFile(apdlog,1,tr("terminate").toUtf8().data(),1);
            break;

        case SINCHRO_WAITING_STATE:
            state.id		= CALL_WAITING_STATE;
            if (parsm->repeat==0){
                statusLine=tr("повторный запуск");
                emit synchrStatus(statusLine);
                emit synchrFailed(parsm->repeat+1);
            }else{
                res					= 2;				// синхронизация не установлена
                statusLine=tr("ошибка:превышение лимита времени");
                emit synchrStatus(statusLine);
            }
            break;

        case CALL_CONFIRM_WAITING_STATE:
            state.id		= CALL_WAITING_STATE;
            res					= 2;									// соединение не установлено           
            statusLine=tr("ошибка:превышение лимита времени");
            emit synchrStatus(statusLine);
            //emit connectStatus( srcAdress, mdmNumber,strStatus(2),msg);
            break;

        case DISCONNECT_CONFIRM_WAITING_STATE:  // подтверждение на разрыв не получено
            state.id		= CALL_WAITING_STATE;//DATA_RECEIVING_STATE;
            res					= 2;									//
            statusLine=tr("error: run out of time limit");
            //emit disconnectStatus(mdmNumber, srcAdress,strStatus(3),msg);
            break;

        case SOS_RECEIVING_STATE:
            state.id		= SINCHRO_TRANSMISSION_STATE;
            state.time		= s_reg.synchro_trans_time * 20;
            is_kdg_to_transmit = true;
            //emit synchrStatus( cfg->master, mdmNumber,strStatus(1),msg);
            break;

        case ALIEN_SOS_RECEIVING_STATE:
            state.id		= ALIEN_SINCHRO_RECEIVING_STATE;
            state.time		= s_reg.synchro_trans_time * 20;
            break;

        case ALIEN_SINCHRO_RECEIVING_STATE:
            state.id		= CALL_WAITING_STATE;
            statusLine.clear();
            emit synchrStatus(statusLine);
            res = 1;			// синхронизация между другой парой модемов установлена
            break;

        case ALIEN_CALL_CONFIRM_RECEIVING_STATE:
            state.id		= CALL_WAITING_STATE;
            statusLine.clear();
            emit synchrStatus(statusLine);
            break;

        case CALL_RECEIVING_STATE:
            state.id		= CALL_CONFIRM_TRANSMISSION_STATE;
            state.time		= s_reg.call_confirm_trans_time * 20;            
            is_kdg_to_transmit = true;
            //emit connectStatus( srcAdress, mdmNumber,strStatus(1),msg);
            break;

        case ALIEN_CALL_RECEIVING_STATE:
            state.id		= ALIEN_CALL_CONFIRM_RECEIVING_STATE;
            state.time		= s_reg.call_confirm_trans_time * 20;
            break;

        case SINCHRO_RECEIVING_STATE:
            state.id		= CALL_WAITING_STATE;
            //emit setSyncSource(srcAdress);
            res = 1;													// синхронизация установлена
            statusLine=tr("Синхронизация установлена");
            emit synchrStatus(statusLine);
            break;

        case CALL_CONFIRM_TRANSMISSION_STATE:
            //if (parsm->duplex != 1)
            {
                state.id		= DATA_RECEIVING_STATE;
                adapt_speed.id = WAIT_EVENTS;
                is_kdg_to_transmit = false;
                res = 1;
                statusLine=tr("Соединение установлено-с");   // на сервере
                parsm->simplex=false;
                if (parsm->switch_mode==1) {
                    parsm->fixFrequencyMode = true;
                    parsm->wait_best_frequency_in_transmission = true;
                }
                emit synchrStatus(statusLine);
                emit connectStatus(server_address,klient_address, parsm->txChan, parsm->rxChan);
                emit connectMode(parsm);
                //logConnectParams(parsm);
                slot.cntFrame	= 1;
                slot.local		= 0;
                memset(rsErrors, 0, 128*8);							// Обнуление массива
            }
            break;

        case CALL_CONFIRM_RECEIVING_STATE:
            state.id		= DATA_RECEIVING_STATE;
            adapt_speed.id = WAIT_EVENTS;
            res = 1;
            statusLine=tr("Соединение установлено-к");  // у клиента
            parsm->simplex=false;
            parsm->adapt_speed_ball_is_mine = true;
            is_kdg_to_transmit = false;
            if (parsm->switch_mode==1) {
                if (parsm->fixFrequencies[0] == parsm->fixFrequencies[1])  parsm->fixFrequencies[1] =  parsm->fixFrequencies[2];
                parsm->fixFrequencyMode = true;
                duplex_state.id		= DATA_RECEIVING_STATE;
                set_info_head_type(ADDITIONAL_FREQUENCY_SHIFT,parsm->fixFrequencies[1],0);
                emit connectStatus(klient_address,server_address, parsm->fixFrequencies[0], parsm->fixFrequencies[1]);
            } else {
                emit connectStatus(klient_address,server_address, parsm->txChan, parsm->rxChan);
            }
            emit synchrStatus(statusLine);
            emit connectMode(parsm);
            //logConnectParams(parsm);
            slot.local		= 0;
            memset(rsErrors, 0, 128*8);							// Обнуление массива
            break;
        }
        intToFile(tcong, 0, tr("     new state.id= ").toUtf8().data(),state.id, 0, 10,' ',1);

    }
    state.time--;
    if (duplex_state.time == 0){
        switch (duplex_state.id)				//Изменить состояние по истечении времени
        {
        case CALL_CONFIRM_TRANSMISSION_STATE:
            if (parsm->duplex == 1) {
                state.id		= DATA_RECEIVING_STATE;
                duplex_state.id		= DATA_RECEIVING_STATE;
                adapt_speed.id = WAIT_EVENTS;
                is_kdg_to_transmit = false;
                res = 1;
                statusLine=tr("Соединение установлено-cд");   // на сервере дуплекс-коннект
                parsm->simplex=false;
                if (parsm->switch_mode==1) {
                    parsm->fixFrequencyMode = true;
                    parsm->wait_best_frequency_in_transmission = true;
                    emit connectStatus(s_reg.addressm,server_address, parsm->fixFrequencies[0], parsm->fixFrequencies[1]);
                }
                if (parsm->switch_mode==2) {
                    emit connectStatus(server_address,klient_address, parsm->chan_number[0], parsm->chan_number[1]);
                }
                emit synchrStatus(statusLine);

                emit connectMode(parsm);
                //logConnectParams(parsm);
                slot.cntFrame	= 1;
                slot.local		= 0;
                memset(rsErrors, 0, 128*8);							// Обнуление массива
            }
            break;
        }
    }
    duplex_state.time--;

    return	res;
}

quint32 TConnect::getDoneProcent(QString &msg)
{
    quint32 res = 0;
    QString	sz,str="";
    if (state.time >= 0){
        switch( state.id )
        {
        case SOS_TRANSMISSION_STATE:
            res	= 100 - (5*state.time)/s_reg.synchro_query_trans_time;
            str = tr("Посылка синхрозапроса");
            break;
        case SINCHRO_WAITING_STATE:
            res	= 100 - (5*state.time)/s_reg.synchro_waiting_time;
            str = tr("Ожидание синхронизации");
            break;
        case SINCHRO_RECEIVING_STATE:
            str = tr("Получение синхроответа");
            res	= 100 - (5*state.time)/s_reg.synchro_waiting_time;
            break;
        case ALIEN_SINCHRO_RECEIVING_STATE:
            str = tr("Система занята");
            res	= 100 - (5*state.time)/s_reg.synchro_waiting_time;
            break;
        case SOS_RECEIVING_STATE:
            str = tr("Получение синхрозапроса");
            res	= 100 - (5*state.time)/s_reg.synchro_query_trans_time;
            break;
        case ALIEN_SOS_RECEIVING_STATE:
            str = tr("Система занята");
            res	= 100 - (5*state.time)/s_reg.synchro_query_trans_time;
            break;
        case SINCHRO_TRANSMISSION_STATE:
            str = tr("Передача синхронизации");
            res	= 100 - (5*state.time)/s_reg.synchro_trans_time;
            break;
        case CALL_TRANSMISSION_STATE:
            str = tr("Запрос на соединение");
            res	= 100 - (5*state.time)/s_reg.call_trans_time;
            break;
        case CALL_CONFIRM_WAITING_STATE:
            str = tr("Ожидание подтверждения");
            res	= 100 - (5*state.time)/s_reg.call_confirm_waiting_time;
            break;
        case CALL_CONFIRM_RECEIVING_STATE:
            if (parsm->duplex != 1 ){
                str = tr("Прием подтверждения1");
                res	= 100 - (5*state.time)/s_reg.call_confirm_waiting_time;
            }
            break;
        case ALIEN_CALL_CONFIRM_RECEIVING_STATE:
            str = tr("Система занята");
            res	= 100 - (5*state.time)/s_reg.call_confirm_waiting_time;
            break;
        case CALL_RECEIVING_STATE:
            str = tr("Запрос соединения");
            res	= 100 - (5*state.time)/s_reg.call_trans_time;
            break;
        case ALIEN_CALL_RECEIVING_STATE:
            str = tr("Система занята");
            res	= 100 - (5*state.time)/s_reg.call_trans_time;
            break;
        case CALL_CONFIRM_TRANSMISSION_STATE:
            str = tr("Подтверждение соединения");
            res	= 100 - (5*state.time)/s_reg.call_confirm_trans_time;
            break;
        case SEND_DISCONNECT_REQUEST:
            str = tr("Запрос на разъединение");
            res	= 100 - (5*state.time)/s_reg.call_trans_time;
            break;
        case DISCONNECT_CONFIRM_WAITING_STATE:
            str = tr("Ожидание подтверждения разъединения");
            res	= 100 - (5*state.time)/s_reg.disconnect_confirm_waiting_time;
            break;
        case RECV_DISCONNECT_REQUEST:
            str = tr("Получение запроса на разъединение");
            res	= 100 - (5*state.time)/s_reg.call_trans_time;
            break;
        case SEND_DISCONNECT_CONFIRM:
            str = tr("Посылка подтверждения на разъединение");
            res	= 100 - (5*state.time)/s_reg.call_trans_time;
            break;
        case RECV_DISCONNECT_CONFIRM:
            str = tr("Разъединение" );
            res	= 100 - (5*state.time)/s_reg.disconnect_confirm_waiting_time;
            break;
        }
    }
    if (duplex_state.time >= 0){
        switch( duplex_state.id )
        {
        case CALL_CONFIRM_WAITING_STATE:
            str = tr("Ожидание подтверждения d");
            res	= 100 - (5*duplex_state.time)/s_reg.call_confirm_waiting_time;
            break;
        case CALL_CONFIRM_RECEIVING_STATE:
            //if (parsm->duplex != 1 )
            if(parsm->switch_mode == 1){
                str = tr("Прием подтверждуния2");
                res	= 100 - (5*duplex_state.time)/s_reg.call_confirm_waiting_time;
            }
            break;
        case CALL_CONFIRM_TRANSMISSION_STATE:
            str = tr("Передача подтверждения");
            res	= 100 - (5*duplex_state.time)/s_reg.call_confirm_trans_time;
            break;
        }
    }
    //if ((res>100)||(res<0)) state.id = CALL_WAITING_STATE;
    if (!str.isEmpty()){
        str = str.append(("  " ) +sz.setNum(res) + tr("%"));
        sz.setNum(state.id);
        msg=str.append(" state="+sz);
        sz.setNum(parsm->txChan);
        msg=str.append(" txChan="+sz);
        sz.setNum(parsm->rxChan);
        msg=str.append(" rxChan="+sz);
    }else msg="";

    return res;
}

void TConnect::time_increment()
{
    generator->next();
}

void TConnect::sendRecvControl()
{
    slotCounter++;
    if (parsm->simplex){
        controlSimplex();
    }else if (parsm->duplex){
        controlDuplex();
    }else{
        controlHalfDuplex();
    }
}

int TConnect::freqIndex_send(int shift)
{
    int ch;
    ch = (pspNumber+ shift + 2*parsm->txPspChannel)  % parsm->startFreqNumber;
    return ch;
}

int TConnect::freqIndex_recv(int shift)
{
    int ch;
    ch = (pspNumber+ shift + 2*parsm->rxPspChannel)  % parsm->startFreqNumber;
    return ch;
}


void TConnect::set_mdm_time(quint32 tm)
{
    int chan;
    generator->set_clock(tm);

    parsm->futureSendIndexes1.clear();
    parsm->futureRecvIndexes1.clear();

    future_psp_numbers->clear();
    for(int ii=0;ii<FUTURE_PSP;ii++){
        chan_refresh(&pspNumber, parsm->startFreqNumber/2);
        future_psp_numbers->append(pspNumber);
        chan =  freqIndex_send(0);

        parsm->futureSendIndexes1.append(chan);
        chan =  freqIndex_recv(0);

        parsm->futureRecvIndexes1.append(chan);
    }
    qDebug() << "init_time= "<<tm;
}

void TConnect::init_mdm_time()
{
    QTime tmd = QTime::currentTime();
    quint32 dtime = static_cast<quint32>(tmd.msecsSinceStartOfDay()/50);     // слоты с начала дня
    set_mdm_time(dtime);
}

uint128_t TConnect::get_mdm_time()
{ 
    uint128_t st;
    st.word32[0] = generator->clock().field[0];
    return st;
}

void TConnect::chan_refresh(int* chan, int chans_all,int shift)
{
    int num=0;
    quint32 t_slot;
    uint128_t slotTime1;

    if (shift==1){
        num = generator->next() % chans_all;
    }else{
        if(shift > 1) generator->skip(shift);
        else if(shift < 0) generator->goback(-shift);
        num=generator->current() % chans_all;
    }

    t_slot = generator->clock().field[0];

    slotTime1.word32[0] = t_slot;
    *chan =num*2 + slotTime1.byte8[0] % 2;
    //    intToFile(tcong, 0, tr(" num= ").toUtf8().data(),num, 10, 10,' ',0);
    //    intToFile(tcong, 0, tr(" slotTime= ").toUtf8().data(),slotTime1.word32[0], 10, 10,' ',0);
    //    intToFile(tcong, 0, tr(" slotTime8= ").toUtf8().data(),slotTime1.byte8[0], 10, 10,' ',0);
    //    intToFile(tcong, 0, tr(" clock8= ").toUtf8().data(),t_slot % 256, 10, 10,' ',0);
    //    intToFile(tcong, 0, tr(" chan= ").toUtf8().data(),*chan, 10, 10,' ',1);
}

void TConnect::chan_refresh1(int* chan)
{
    chan_refresh(&pspNumber, parsm->startFreqNumber/2);
    *chan = future_psp_numbers->first();
    future_psp_numbers->removeFirst();
    future_psp_numbers->append(pspNumber);

    int num =  freqIndex_send(0);
    parsm->futureSendIndexes1.removeFirst();
    parsm->futureSendIndexes1.append(num);

    num =  freqIndex_recv(0);

    parsm->futureRecvIndexes1.removeFirst();
    parsm->futureRecvIndexes1.append(num);
}

void TConnect::controlSimplex()
{
    if (is_kdg_to_transmit){
        mSending=true;
        mRecieving=false;
    }else{
        mSending=false;
        mRecieving=true;
    }
}

void TConnect::controlHalfDuplex()
{
    quint32 time	= generator->clock().field[0];
    if (time%2 == parsm->half_duplex_num%2){
        mSending = true;
        mRecieving=false;
    }else{
        mSending = false;
        mRecieving=true;
    }
}

void TConnect::controlDuplex()
{
    mSending = true;
    mRecieving=true;
}

void TConnect::setDuplexMode(connect_Pars_T *pars)
{
    if (pars->duplex==1) {
        mSending = true;
        mRecieving=true;
    }
}

bool TConnect::snd()
{
    bool res = true;
    if (cprop->snd) res = true;
    else{
        if (parsm->wait_best_frequency_in_transmission) res = false;
        else res = mSending;
    }
    return res;
}

bool TConnect::rcv()
{
    return mRecieving;
}

void TConnect::setLoop_fixFrequecyMode(bool nrch)
{
    if (cprop->data_exchange){
        parsm->fixFrequencyMode = nrch;
    }
}

void TConnect::logConnectParams(connect_Pars_T* pars)
{
    if (tcong == nullptr) return;
    strToFile(tcong,0,tr(" ").toUtf8().data(),1);
    intToFile(tcong, 1, tr("     CONNECT: mNumber= ").toUtf8().data(),s_reg.addressm, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     pars.server_num= ").toUtf8().data(),pars->server_num, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     band= ").toUtf8().data(),pars->band, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     speed= ").toUtf8().data(),pars->speed, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     switch_mode= ").toUtf8().data(),pars->switch_mode, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     chan_number1= ").toUtf8().data(),pars->chan_number[0], 0, 10,' ',1);
    intToFile(tcong, 0, tr("     chan_number2= ").toUtf8().data(),pars->chan_number[1], 0, 10,' ',1);
    intToFile(tcong, 0, tr("     fixFrequencyMode= ").toUtf8().data(),pars->fixFrequencyMode, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     freq_number1= ").toUtf8().data(),pars->fixFrequencies[0], 0, 10,' ',1);
    intToFile(tcong, 0, tr("     freq_number2= ").toUtf8().data(),pars->fixFrequencies[1], 0, 10,' ',1);
    intToFile(tcong, 0, tr("     duplex= ").toUtf8().data(),pars->duplex, 0, 10,' ',1);
    intToFile(tcong, 0, tr("     break_enabled= ").toUtf8().data(),pars->break_enabled, 0, 10,' ',0);
    intToFile(tcong, 0, tr(" auto_end= ").toUtf8().data(),pars->break_on_50, 0, 10,' ',1);
    strToFile(tcong,0,tr(" ").toUtf8().data(),1);
}

void TConnect::setFrchConnect()
{
    QString str;
    qDebug() << "setFrchConnect()";
    if (parsm->switch_mode==0){
        if (parsm->startup){
            parsm->fixFrequencies[0] = FIX_FREQ_NUMBER1;
            parsm->fixFrequencies[1] = FIX_FREQ_NUMBER2;
        }else{
            parsm->fixFrequencies[0] = FIX_FREQ_NUMBER2;
            parsm->fixFrequencies[1] = FIX_FREQ_NUMBER1;
        }
        parsm->fixFrequencyMode = true;
        parsm->duplex = 1;
        parsm->simplex = 0;
        parsm->wait_best_frequency_in_transmission = false;
    }
    state.id = DATA_RECEIVING_STATE;
    emit connectStatus(s_reg.addressm,parsm->server_num , parsm->fixFrequencies[0], parsm->fixFrequencies[1]);
    str=tr("Соединение на фиксированных частотах");
    emit synchrStatus(str);
}

void TConnect::setDataMode(bool st)
{
    if (st){
        state.id = DATA_RECEIVING_STATE;
        intToFile(tcong, 1, tr(" set_loop state.id= ").toUtf8().data(),state.id, 0, 10,' ',1);

    }else{
        state.id = CALL_WAITING_STATE;
    }
}

bool TConnect::serviceKdg()
{
    bool service_kdg = (state.id != DATA_RECEIVING_STATE);
    if (cprop->data_exchange) service_kdg = false;
    return service_kdg;
}

bool TConnect::speedAdaptElevated()
{
    bool res=false;
    if ((parsm->speed_adapt)||(parsm->freq_adapt)){
            res=true;
    }
    return (res && slow_down_on_break);
}

void TConnect::endOfConnection(QString status)
{
    state.id		= CALL_WAITING_STATE;
    adapt_speed.id = NO_ADAPT;
    info_head_type = SPEED_ADAPTATION;
    set_info_head_type(SPEED_ADAPTATION,0,0,100);
    parsm->adapt_speed_ball_is_mine = false;
    emit connectStatus(0,0, 0,0);
    parsm->fixFrequencyMode = false;
    parsm->simplex = true;
    parsm->startup = false;
    emit synchrStatus(status);
    //emit disconnectStatus(srcAdress,mdmNumber,strStatus(1),msg);
}

void TConnect::breakTheSpeed(QString status)
{
    int var = ads->getBaseIndex();
    ads->initAdaptation(var);
    adapt_speed.id = WAIT_EVENTS;
    set_info_head_type(SPEED_ADAPTATION,1,1,99);
    emit synchrStatus(status);
    state.id		= DATA_RECEIVING_STATE;
    if (parsm->half_duplex_num == 1)
        parsm->adapt_speed_ball_is_mine = true; // у инициатора соединения
    else
        parsm->adapt_speed_ball_is_mine = false;
    emit setWorkingVariant(var);  // -> DceSettings::set_cbVariantIndex
    //slow_down_on_break=false;
}

void TConnect::msg_to_Status()
{
    QString str,statusLine,str1;
    if (state.id	== DATA_RECEIVING_STATE){
        if (speed_adaptation.enable || freq_adaptation.enable){
            str1 = QString(" ind=%1 ball=%2 as.id=%3 ctrl.id=%4").arg(parsm->half_duplex_num)
                    .arg(parsm->adapt_speed_ball_is_mine)
                    .arg(adapt_speed.id).arg(common_adapt_state.id);
            if (parsm->adapt_current == SPEED){
                statusLine=tr(" SPEED_ADAPTATION");
            }
            else if (parsm->adapt_current == FREQUENCY){
                str =  tr(" FREQUENCY_ADAPTATION");
            }else{
                str =  tr(" NONE_ADAPTATION");
            }
            statusLine.append(str);
            statusLine.append(str1);
            emit synchrStatus(statusLine);
        }
    }
}

void TConnect::init_adapt_s(int st)
{
    parsm->speed_adapt = st;
}

void TConnect::init_adapt_f(int st)
{
    if (parsm->fixFrequencyMode){
        parsm->freq_adapt = false;
    }else{
        parsm->freq_adapt = st;
        if (st) ads->clearAllStatistics();
    }
}

bool TConnect::ballIsMine()
{
    return parsm->adapt_speed_ball_is_mine;
}

bool TConnect::newAdaptStep()
{
    return (adapt_speed.id == STATISTICS_CALCULATION);
}

bool TConnect::testPreviousChange()
{
    return ((adapt_speed.id == NEW_SPEED_STATISTICS_CALC));
}

void TConnect::setAdaptState(quint8 htype)
{
    adapt_speed.id = htype;
}

void TConnect::set_info_head_type(quint8 htype, quint8 par1, quint8 par2,quint8 id)
{
    info_head_type = htype;
    info_head_par1 = par1;
    info_head_par2 = par2;
    intToFile(tcong, 1, tr(" SET_INFO_HEAD type= ").toUtf8().data(),htype, 0, 10,' ',0);
    intToFile(tcong, 0, tr(" par1= ").toUtf8().data(),par1, 0, 10,' ',0);
    intToFile(tcong, 0, tr(" par2= ").toUtf8().data(),par2, 0, 10,' ',0);
    intToFile(tcong, 0, tr(" id= ").toUtf8().data(),id, 0, 10,' ',1);
}

void TConnect::setSlowDownOnBreak(bool state)
{
    slow_down_on_break=state;
}

void TConnect::setModemProp(MODEM_PROP* pr)
{
    cprop = pr;
}


