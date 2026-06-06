#include "a15680/m15680.h"
QVector<double> m15680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
