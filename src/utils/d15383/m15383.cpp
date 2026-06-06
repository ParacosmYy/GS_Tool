#include "d15383/m15383.h"
QVector<double> m15383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
