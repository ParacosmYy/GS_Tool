#include "d27783/m27783.h"
QVector<double> m27783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
