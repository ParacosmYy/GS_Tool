#include "f36485/m36485.h"
QVector<double> m36485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
