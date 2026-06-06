#include "h8047/m8047.h"
QVector<double> m8047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
