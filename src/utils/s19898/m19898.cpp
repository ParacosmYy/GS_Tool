#include "s19898/m19898.h"
QVector<double> m19898::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
