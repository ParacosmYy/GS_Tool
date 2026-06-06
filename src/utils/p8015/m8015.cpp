#include "p8015/m8015.h"
QVector<double> m8015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
