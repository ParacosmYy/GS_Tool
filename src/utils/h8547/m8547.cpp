#include "h8547/m8547.h"
QVector<double> m8547::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
