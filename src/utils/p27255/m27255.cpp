#include "p27255/m27255.h"
QVector<double> m27255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
