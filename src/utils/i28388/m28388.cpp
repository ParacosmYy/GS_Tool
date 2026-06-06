#include "i28388/m28388.h"
QVector<double> m28388::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
