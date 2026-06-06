#include "k9550/m9550.h"
QVector<double> m9550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
