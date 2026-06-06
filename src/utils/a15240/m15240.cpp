#include "a15240/m15240.h"
QVector<double> m15240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
