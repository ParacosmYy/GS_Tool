#include "a30600/m30600.h"
QVector<double> m30600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
