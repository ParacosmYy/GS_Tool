#include "e20604/m20604.h"
QVector<double> m20604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
