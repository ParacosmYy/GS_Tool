#include "o8574/m8574.h"
QVector<double> m8574::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
