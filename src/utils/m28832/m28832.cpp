#include "m28832/m28832.h"
QVector<double> m28832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
