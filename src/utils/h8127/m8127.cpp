#include "h8127/m8127.h"
QVector<double> m8127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
