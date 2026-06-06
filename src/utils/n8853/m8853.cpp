#include "n8853/m8853.h"
QVector<double> m8853::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
