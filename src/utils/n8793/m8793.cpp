#include "n8793/m8793.h"
QVector<double> m8793::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
