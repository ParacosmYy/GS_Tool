#include "p16535/m16535.h"
QVector<double> m16535::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
