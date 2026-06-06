#include "i36108/m36108.h"
QVector<double> m36108::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
