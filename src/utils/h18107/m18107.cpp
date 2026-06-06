#include "h18107/m18107.h"
QVector<double> m18107::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
