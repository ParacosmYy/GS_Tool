#include "d25383/m25383.h"
QVector<double> m25383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
