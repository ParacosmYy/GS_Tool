#include "l19251/m19251.h"
QVector<double> m19251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
