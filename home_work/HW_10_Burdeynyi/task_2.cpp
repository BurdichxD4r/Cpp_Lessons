/* TODO: Класс Покупатель: Фамилия, Имя, Отчество, Адрес, Номер кредитной карточки,
        Номер банковского счета;
*/

#include <iostream>
#include <string>

class Customers{
private:
    std::string m_lastName;
    std::string m_name;
    std::string m_patronymic;
    std::string m_creditCard;
    std::string m_bank;

public:
    void setLastName(std::string lastName){m_lastName = std::move(lastName);}
    void setName(std::string name){m_name = std::move(name);}
    void setPatronymic(std::string patronymic){m_patronymic = std::move(patronymic);}
    void setCreditCard(std::string creditCard){m_creditCard = std::move(creditCard);}
    void setBank(std::string bank){m_bank = std::move(bank);}

    std::string getLastName() const {return m_lastName;}
    std::string getName() const {return m_name;}
    std::string getPatronymic() const {return m_patronymic;}
    std::string getCreditCard() const {return m_creditCard;}
    std::string getBank() const {return m_bank;}
};

int main(){
    Customers customer;
    std::string lastName, name, patronymic, creditCard, bank;

    std::cout << "Введите данные:\n1. Фамилия\n2. Имя\n3. Отчество\n4. "
                 "Номер кредитной карты\n5. Номер банковского счета\n";
    std::cin >> lastName;
    std::cin >> name;
    std::cin >> patronymic;
    std::cin >> creditCard;
    std::cin >> bank;

    customer.setLastName(lastName);
    customer.setName(name);
    customer.setPatronymic(patronymic);
    customer.setCreditCard(creditCard);
    customer.setBank(bank);

    std::cout << customer.getLastName() << '\n'
              << customer.getName() << '\n'
              << customer.getPatronymic() << '\n'
              << customer.getCreditCard() << '\n'
              << customer.getBank() << std::endl;

    return 0;
}
