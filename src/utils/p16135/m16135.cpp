#include "p16135/m16135.h"
QVector<double> m16135::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
