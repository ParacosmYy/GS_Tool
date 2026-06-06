#include "m36252/m36252.h"
QVector<double> m36252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
