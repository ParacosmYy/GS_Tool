#include "i15308/m15308.h"
QVector<double> m15308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
