#include "i36828/m36828.h"
QVector<double> m36828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
