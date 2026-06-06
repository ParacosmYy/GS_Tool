#include "f17025/m17025.h"
QVector<double> m17025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
