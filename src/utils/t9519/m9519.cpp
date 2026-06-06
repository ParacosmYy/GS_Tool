#include "t9519/m9519.h"
QVector<double> m9519::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
