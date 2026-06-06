#include "s16278/m16278.h"
QVector<double> m16278::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
