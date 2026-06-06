#include "f25025/m25025.h"
QVector<double> m25025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
