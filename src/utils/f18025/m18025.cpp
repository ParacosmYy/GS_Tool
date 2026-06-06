#include "f18025/m18025.h"
QVector<double> m18025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
