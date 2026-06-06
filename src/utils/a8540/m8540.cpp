#include "a8540/m8540.h"
QVector<double> m8540::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
