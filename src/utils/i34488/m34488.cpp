#include "i34488/m34488.h"
QVector<double> m34488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
