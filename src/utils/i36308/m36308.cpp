#include "i36308/m36308.h"
QVector<double> m36308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
