#include "p16015/m16015.h"
QVector<double> m16015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
