#include "i28908/m28908.h"
QVector<double> m28908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
