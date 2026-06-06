#include "a15100/m15100.h"
QVector<double> m15100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
