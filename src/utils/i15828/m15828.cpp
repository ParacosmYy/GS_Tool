#include "i15828/m15828.h"
QVector<double> m15828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
