#include "a28160/m28160.h"
QVector<double> m28160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
