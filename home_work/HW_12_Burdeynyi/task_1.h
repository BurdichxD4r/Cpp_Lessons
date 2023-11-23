#include <iostream>
#include <vector>

class Menu;

class CashRegister;

class Retailitem{
private:
    static int s_i;
    std::string m_product;
    double m_price;
    int m_index;
    short m_count;
public:
    Retailitem(std::string product, double price, short count, int index = s_i)
        :m_product(product), m_price(price), m_count(count), m_index(index){++s_i;}

    void deleteProductForStoreHouse(Retailitem & item){--item.m_count;}

    friend std::ostream & operator<<(std::ostream & out ,const Retailitem & item);

    friend CashRegister;

    friend Menu;
};

class CashRegister{
private:
    std::vector<Retailitem *> m_arrayItem;
    double m_result;
public:
    CashRegister(double result = 0):m_result(result){}

    void buyItem(Retailitem & item){
        if (item.m_count == 0){
            std::cout << "Недостаточно товара на складе!\n";
        }else{
            m_arrayItem.push_back(& item);
        }
    }

    void getTotal(){
        m_result = 0;
        for (int i = 0; i < std::size(m_arrayItem); ++i){
            m_result += (* m_arrayItem[i]).m_price;
        }
        std::cout << "Стоимость товаров в корзине - " << m_result << "$\n";
    }

    void showItems(){
        for (int i = 0; i < m_arrayItem.size(); ++i){
            std::cout << (* m_arrayItem[i]).m_product << "\t"
                      << (* m_arrayItem[i]).m_price << "$\n";
        }
    }

    void buy(){
        for (int i = 0; i < m_arrayItem.size(); ++i){
            (* m_arrayItem[i]).deleteProductForStoreHouse(* m_arrayItem[i]);
        }
        m_arrayItem.clear();
    }

    void clear(){
        m_arrayItem.clear();
    }

    friend Menu;
};

class Menu{
private:
    int value;
    int index;
public:
    CashRegister user;

    void showMenu(){
        std::cout << "1. Показать меню\n2. Показать список товаров\n3. Добавить товар в корзину\n"
                     "4. Показать список товаров в корзине\n5. Показать стоимость товаров в корзине\n"
                     "6. Купить товар(ы) в корзине\n7. Отчистить корзину\n8. Выход\n";
    }

    void mainMenu(Retailitem item1, Retailitem item2, Retailitem item3){
        value = 0;

        showMenu();

        while (value != 8){
            std::cout << " => ";
            std::cin >> value;

            switch (value){
            case 1:
                break;
            case 2:
                std::cout << "\n\t\tОписание\tКол-во на складе\tЦена\n";
                std::cout << item1 << item2 << item3 << "\n";
                break;
            case 3:
                std::cout << "Введите индекс товара\n => ";
                std::cin >> index;

                switch (index){
                case 1:
                    user.buyItem(item1);
                    break;
                case 2:
                    user.buyItem(item2);
                    break;
                case 3:
                    user.buyItem(item3);
                    break;
                default:
                    std::cout << "Такого товара нет!\n";
                    break;
                }
                break;
            case 4:
                user.showItems();
                break;
            case 5:
                user.getTotal();
                break;
            case 6:
                user.buy();
                break;
            case 7:
                user.clear();
                break;
            case 8:
                exit(0);
                break;
            default:
                std::cout << "Введено некорректное значение!\n";
                break;
            }
        }
    }
};
