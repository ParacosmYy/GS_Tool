#include "k27050/m27050.h"
QVector<double> m27050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
