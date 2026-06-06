#include "a25860/m25860.h"
QVector<double> m25860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
