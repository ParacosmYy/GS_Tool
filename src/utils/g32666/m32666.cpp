#include "g32666/m32666.h"
QVector<double> m32666::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
