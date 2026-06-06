#include "m15272/m15272.h"
QVector<double> m15272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
