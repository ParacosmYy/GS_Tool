#include "g7806/m7806.h"
QVector<double> m7806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
