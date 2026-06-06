#include "f8025/m8025.h"
QVector<double> m8025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
