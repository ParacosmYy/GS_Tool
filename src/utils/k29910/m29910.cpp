#include "k29910/m29910.h"
QVector<double> m29910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
