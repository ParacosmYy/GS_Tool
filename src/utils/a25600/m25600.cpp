#include "a25600/m25600.h"
QVector<double> m25600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
