#include "a15880/m15880.h"
QVector<double> m15880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
