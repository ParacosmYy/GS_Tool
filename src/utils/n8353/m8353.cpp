#include "n8353/m8353.h"
QVector<double> m8353::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
