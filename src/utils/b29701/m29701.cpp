#include "b29701/m29701.h"
QVector<double> m29701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
