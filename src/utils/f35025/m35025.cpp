#include "f35025/m35025.h"
QVector<double> m35025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
