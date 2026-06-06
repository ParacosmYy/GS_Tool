#include "i30588/m30588.h"
QVector<double> m30588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
