#include "f33025/m33025.h"
QVector<double> m33025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
