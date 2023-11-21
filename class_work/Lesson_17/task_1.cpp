#include <iostream>

class Point{
    double m_x, m_y;
public:
    Point(double x = 0.0, double y = 0.0):m_x(x), m_y(y){}
    friend std::ostream & operator<<(std::ostream & out, const Point & point);
};

std::ostream & operator<<(std::ostream & out, const Point & point){
    out << "Point(" << point.m_x << ", " << point.m_y << ")\n";
    return out;
}

int main(){
    Point point1(5.5, 6.0);
    std::cout << point1;
    return 0;
}
