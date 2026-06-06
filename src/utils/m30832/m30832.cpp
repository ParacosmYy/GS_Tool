#include "m30832/m30832.h"
QVector<double> m30832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
