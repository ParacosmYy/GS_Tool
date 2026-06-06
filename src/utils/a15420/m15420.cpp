#include "a15420/m15420.h"
QVector<double> m15420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
