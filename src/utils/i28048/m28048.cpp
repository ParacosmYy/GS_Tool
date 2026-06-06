#include "i28048/m28048.h"
QVector<double> m28048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
