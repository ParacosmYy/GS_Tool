#include "i27488/m27488.h"
QVector<double> m27488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
