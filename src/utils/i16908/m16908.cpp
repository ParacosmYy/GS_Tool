#include "i16908/m16908.h"
QVector<double> m16908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
