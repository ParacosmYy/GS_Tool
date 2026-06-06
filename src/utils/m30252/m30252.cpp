#include "m30252/m30252.h"
QVector<double> m30252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
